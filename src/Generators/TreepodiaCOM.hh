<?hh // strict
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\XMLGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Helper\Models\KeyValue;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;
use Plenty\Modules\Category\Contracts\CategoryBranchRepositoryContract;
use Plenty\Modules\Category\Contracts\CategoryRepository;
use Plenty\Modules\Item\Producer\Contracts\ProducerRepositoryContract;
use Plenty\Modules\Item\Producer\Models\Producer;
use Plenty\Modules\Category\Models\Category;

class TreepodiaCOM extends XMLGenerator
{
	/**
	 * @var string
	 */
	protected string $version = '1.0';

	/**
	 * @var string
	 */
	protected string $encoding = 'UTF-8';

	/**
	 * @var bool
	 */
	protected bool $formatOutput = true;

	/**
	 * @var bool
	 */
	protected bool $preserveWhiteSpace = true;

	/*
     * @var ElasticExportHelper
     */
    private ElasticExportHelper $elasticExportHelper;

	/*
	 * @var ArrayHelper
	 */
	private ArrayHelper $arrayHelper;

	/**
     * CategoryBranchRepositoryContract $categoryBranchRepository
     */
    private CategoryBranchRepositoryContract $categoryBranchRepository;

	/**
     * CategoryRepository $categoryRepository
     */
    private CategoryRepository $categoryRepository;

    /**
     * ProducerRepositoryContract $producerRepository
     */
    private ProducerRepositoryContract $producerRepository;

	/**
     * TreepodiaDE constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     * @param CategoryBranchRepositoryContract $categoryBranchRepository
     * @param CategoryRepository $categoryRepository
     * @param ProducerRepositoryContract $producerRepository
     */
    public function __construct(
    	ElasticExportHelper $elasticExportHelper, 
    	ArrayHelper $arrayHelper, 
    	CategoryBranchRepositoryContract $categoryBranchRepository,
    	CategoryRepository $categoryRepository,
    	ProducerRepositoryContract $producerRepository
    )
    {
        $this->elasticExportHelper = $elasticExportHelper;
		$this->arrayHelper = $arrayHelper;
		$this->categoryBranchRepository = $categoryBranchRepository;
		$this->categoryRepository = $categoryRepository;
		$this->producerRepository = $producerRepository;

		$this->init('products');
    }

	/**
	 * @param mixed $resultData
	 */
	protected function generateContent(mixed $resultData, array<FormatSetting> $formatSettings = []):void
	{
		if($resultData instanceof RecordList)
		{
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

			foreach($resultData as $item)
			{
				$product = $this->createElement('product');
				$this->root()->appendChild($product);
				
				// sku
				$product->appendChild($this->createElement('sku', $item->itemBase->id));

				// price
				$product->appendChild($this->createElement('price', number_format($this->elasticExportHelper->getPrice($item), 2)));

				// name
				$product->appendChild($this->createElement('name', $this->elasticExportHelper->getName($item, $settings)));

				// commodity
				$category = $this->getTopLevelCategory($item, $settings);
				if($category instanceof Category)
				{
					foreach($category->details as $detail)
					{
						if($detail->lang == $settings->get('lang'))
						{
							$product->appendChild($this->createElement('commodity', $detail->name));			
						}
					}
				}
				

				// description
				$product->appendChild($description = $this->createElement('description'));
				$description->appendChild($this->createCDATASection($this->elasticExportHelper->getDescription($item, $settings)));

				// brand-name, brand-logo
				if((int) $item->itemBase->producerId > 0)
				{
					$producer = $this->getProducer($item->itemBase->producerId);

					if($producer instanceof Producer)
					{
						if(strlen($producer->external_name) > 0)
						{
							$product->appendChild($brandName = $this->createElement('brand-name'));
							$brandName->appendChild($this->createCDATASection($producer->external_name));
						}
						elseif(strlen($producer->name) > 0)
						{
							$product->appendChild($brandName = $this->createElement('brand-name'));
							$brandName->appendChild($this->createCDATASection($producer->name));
						}

						if(strlen($producer->logo) > 0)
						{
							$product->appendChild($this->createElement('brand_logo', $producer->logo));
						}
					}
				}

				// page-url
				$product->appendChild($pageUrl = $this->createElement('page-url'));
				$pageUrl->appendChild($this->createCDATASection($this->elasticExportHelper->getUrl($item, $settings, false)));

				// image-url
				foreach($this->elasticExportHelper->getImageList($item, $settings) as $image)
				{
					$product->appendChild($this->createElement('image-url', $image));
				}

				// catch-phrase
				foreach($this->getCatchPhraseList($item) as $catchPhrase)
				{
					$product->appendChild($this->createElement('catch-phrase', htmlspecialchars($catchPhrase)));
				}

				// free-shipping
				if($this->elasticExportHelper->getShippingCost($item, $settings) <= 0.00)
				{
					$product->appendChild($this->createElement('free-shipping', 1));
				}

				// youtubetag, Video-Sitemaptag
				foreach($this->getKeywords($item, $settings) as $keyword)
				{
					$product->appendChild($this->createElement('youtubetag', htmlspecialchars(trim($keyword))));
					$product->appendChild($this->createElement('Video-Sitemaptag', htmlspecialchars(trim($keyword))));		
				}
			}

			$this->build();
		}
	}

	/**
	 * Get the top level category.
	 * @param Record item
	 * @param KeyValue settings
	 * @return ?Category
	 */
	public function getTopLevelCategory(Record $item, KeyValue $settings):?Category
	{
		$lang = $settings->get('lang') ? $settings->get('lang') : 'de';

		$categoryBranch = $this->categoryBranchRepository->findCategoryBranch($item->variationStandardCategory->categoryId, $settings->get('lang'), $settings->get('plentyId'));

		if(!is_null($categoryBranch))
		{
			$categoryList = ImmVector{
				$categoryBranch->plenty_category_branch_category6_id,
				$categoryBranch->plenty_category_branch_category5_id,
				$categoryBranch->plenty_category_branch_category4_id,
				$categoryBranch->plenty_category_branch_category3_id,
				$categoryBranch->plenty_category_branch_category2_id,
				$categoryBranch->plenty_category_branch_category1_id
			};

			$categoryId = $categoryList
						->filter($catId ==> $catId > 0)
						->firstValue();

			return $this->categoryRepository->get((int) $categoryId, $lang);
		}

		return null;
	}

	/**
	 * Get producer.
	 * @param int producerId
	 * @return Producer
	 */
	public function getProducer(int $producerId):mixed
	{
		return $this->producerRepository->findById($producerId);
	}

	/**
	 * Get catch phrase list.
	 * @param Record $item
	 * @return array<string>
	 */
	private function getCatchPhraseList(Record $item):array<string>
	{
		$list = ImmVector{
			$item->itemBase->free1,
			$item->itemBase->free2,
			$item->itemBase->free3,
			$item->itemBase->free4,
		};

		return $list
				->filter(($free) ==> strlen($free) > 0)
				->toArray();
	}

	/**
	 * Get keywords.
	 * @param Record $item
	 * @return array<string>
	 */
	public function getKeywords(Record $item, KeyValue $settings):array<string>
	{
		$list = explode(',', $item->itemDescription->keywords);

		$category = $this->getTopLevelCategory($item, $settings);
		if($category instanceof Category)
		{
			foreach($category->details as $detail)
			{
				if($detail->lang == $settings->get('lang'))
				{
					$list = array_merge($list, explode(',', $detail->metaKeywords));			
				}
			}
		}

		return $list;
	}
}