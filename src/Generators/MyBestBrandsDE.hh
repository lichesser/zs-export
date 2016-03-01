<?hh // strict
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;
use Plenty\Modules\Helper\Models\KeyValue;
use Plenty\Modules\Item\Attribute\Contracts\AttributeValueLangRepositoryContract;
use Plenty\Modules\Item\Attribute\Models\AttributeValueLang;
use Plenty\Modules\Character\Contracts\CharacterSelectionRepositoryContract;
use Plenty\Modules\Character\Models\CharacterSelection;

class MyBestBrandsDE extends CSVGenerator
{
	/*
     * @var ElasticExportHelper
     */
    private ElasticExportHelper $elasticExportHelper;

	/*
	 * @var ArrayHelper
	 */
	private ArrayHelper $arrayHelper;

	/**
	 * AttributeValueLangRepositoryContract $attributeValueLangRepository
	 */
	private AttributeValueLangRepositoryContract $attributeValueLangRepository;

	/**
	 * CharacterSelectionRepositoryContract $characterSelectionRepository
	 */
	private CharacterSelectionRepositoryContract $characterSelectionRepository;

	/**
	 * @var array<int,mixed>
	 */
	private array<int,array<string,string>>$itemPropertyCache = [];

	/**
     * Geizhals constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     * @param AttributeValueLangRepositoryContract $attributeValueLangRepository
     * @param CharacterSelectionRepositoryContract $characterSelectionRepository
     */
    public function __construct(
    	ElasticExportHelper $elasticExportHelper, 
    	ArrayHelper $arrayHelper, 
    	AttributeValueLangRepositoryContract $attributeValueLangRepository,
    	CharacterSelectionRepositoryContract $characterSelectionRepository
    )
    {
		$this->elasticExportHelper = $elasticExportHelper;
		$this->arrayHelper = $arrayHelper;
		$this->attributeValueLangRepository = $attributeValueLangRepository;
		$this->characterSelectionRepository = $characterSelectionRepository;
    }

	/**
	 * @param mixed $resultData
	 */
	protected function generateContent(mixed $resultData, array<FormatSetting> $formatSettings = []):void
	{
		if($resultData instanceof RecordList)
		{
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

			$this->setDelimiter(";");

			$this->addCSVContent([
				'ProductID',
				'ProductCategory',
				'Deeplink',
				'ProductName',
				'ImageUrl',
				'ProductDescription',
				'BrandName',
				'Price',
				'PreviousPrice',
				'AvailableSizes',
				'Tags',
				'EAN',
				'LastUpdate',
				'UnitPrice',
				'RetailerAttributes',
				'Color',
			]);

			$rows = [];

			foreach($resultData as $item)
			{
				if(!array_key_exists($item->itemBase->id, $rows))
				{
					$rows[$item->itemBase->id] = $this->getMain($item, $settings);
				}

				if(array_key_exists($item->itemBase->id, $rows) && $item->variationBase->attributeValueSetId > 0)
				{
					$variationAttributes = $this->getVariationAttributes($item, $settings);

					if(array_key_exists('Color', $variationAttributes))
					{
						$rows[$item->itemBase->id]['Color'] = array_unique(array_merge($rows[$item->itemBase->id]['Color'], $variationAttributes['Color']));
					}

					if(array_key_exists('Size', $variationAttributes))
					{
						$rows[$item->itemBase->id]['AvailableSizes'] = array_unique(array_merge($rows[$item->itemBase->id]['AvailableSizes'], $variationAttributes['Size']));
					}
				}
				elseif(array_key_exists($item->itemBase->id, $rows))
				{
					$itemPropertyList = $this->getItemPropertyList($item, $settings);

					foreach($itemPropertyList as $key => $value)
					{
						switch($key)
						{
							case 'color':
								$rows[$item->itemBase->id]['Color'] = array_unique(array_push($rows[$item->itemBase->id]['Color'], $value));								
								break;

							case 'available_sizes':
								$rows[$item->itemBase->id]['AvailableSizes'] = array_unique(array_push($rows[$item->itemBase->id]['AvailableSizes'], $value));								
								break;
						}
					}				
				}
			}			

			foreach($rows as $data)
			{
				if(array_key_exists('Color', $data) && is_array($data['Color']))
				{
					$data['Color'] = implode(';', $data['Color']);
				}

				if(array_key_exists('AvailableSizes', $data) && is_array($data['AvailableSizes']))
				{
					$data['AvailableSizes'] = implode(', ', $data['AvailableSizes']);
				}

				$this->addCSVContent(array_values($data));
			}			
		}
	}

	/**
	 * Get main information.
	 * @param  Record $item
	 * @param  KeyValue $settings
	 * @return array<string,mixed>
	 */
	private function getMain(Record $item, KeyValue $settings):array<string,mixed>
	{
		$itemPropertyList = $this->getItemPropertyList($item, $settings);

		$productName = array_key_exists('itemName', $itemPropertyList) && strlen((string) $itemPropertyList['itemName']) ? 
							$this->elasticExportHelper->cleanName((string) $itemPropertyList['itemName'], $settings->get('nameMaxLength')) : 
							$this->elasticExportHelper->getName($item, $settings);

		$data = [
			'ProductID' 			=> $item->itemBase->id,
			'ProductCategory' 		=> str_replace(array('&', '/'), array('und', ' '), $this->elasticExportHelper->getCategory($item, $settings)),
			'Deeplink' 				=> $this->elasticExportHelper->getUrl($item, $settings, true, false),
			'ProductName'			=> $productName,
			'ImageUrl' 				=> $this->elasticExportHelper->getMainImage($item, $settings),
			'ProductDescription' 	=> $this->elasticExportHelper->getDescription($item, $settings),
			'BrandName'				=> $item->itemBase->producer,
			'Price'					=> number_format($this->elasticExportHelper->getPrice($item, $settings), 2, ',', ''),
			'PreviousPrice'			=> number_format($item->variationRecommendedRetailPrice->price > $this->elasticExportHelper->getPrice($item, $settings) ? $item->variationRecommendedRetailPrice->price : 0, 2, ',', ''),					
			'Tags'					=> $item->itemDescription->keywords,
			'EAN'					=> $item->variationBarcode->code,
			'LastUpdate'			=> $item->itemBase->lastUpdateTimestamp,
			'UnitPrice'				=> $this->elasticExportHelper->getBasePrice($item, $settings),
			'RetailerAttributes'	=> $item->itemBase->storeSpecial == 2 ? 'new-arrival' : '',
			'AvailableSizes'		=> [],
			'Color'					=> [],
		];

		return $data;
	}

	/**
	 * Get variation attributes.
	 * @param  Record   $item
	 * @param  KeyValue $settings
	 * @return array<string,string>
	 */
	private function getVariationAttributes(Record $item, KeyValue $settings):array<string,string>
	{
		$variationAttributes = [];

		foreach($item->variationAttributeValueList as $variationAttribute)
		{
			$attributeValueLang = $this->attributeValueLangRepository->findAttributeValue($variationAttribute->attributeValueId, $settings->get('lang'));

			if($attributeValueLang instanceof AttributeValueLang)
			{
				$amazonVariation = $attributeValueLang->attributeValue->attribute->amazon_variation == 'Color';
				if($attributeValueLang->attributeValue->attribute->amazon_variation)
				{
					$variationAttributes[$attributeValueLang->attributeValue->attribute->amazon_variation][] = $attributeValueLang->name;
				}
			}
		}

		return $variationAttributes;
	}

	/**
	 * Get item properties. 
	 * @param 	Record $item
	 * @param  KeyValue $settings
	 * @return array<string,string>
	 */
	protected function getItemPropertyList(Record $item, KeyValue $settings):array<string,string>
	{
		if(!array_key_exists($item->itemBase->id, $this->itemPropertyCache))
		{
			$characterMarketComponentList = $this->elasticExportHelper->getItemCharactersByComponent($item, $settings);

			$list = [];

			if(count($characterMarketComponentList))
			{
				foreach($characterMarketComponentList as $data)
				{
					if((string) $data['characterValueType'] != 'file' && (string) $data['characterValueType'] != 'empty' && (string) $data['externalComponent'] != "0")
					{
						if((string) $data['characterValueType'] == 'selection')
						{
							$characterSelection = $this->characterSelectionRepository->findCharacterSelection((int) $data['characterValue']);
							if($characterSelection instanceof CharacterSelection)
							{
								$list[(string) $data['externalComponent']] = (string) $characterSelection->name;			
							}
						}
						else
						{
							$list[(string) $data['externalComponent']] = (string) $data['characterValue'];	
						}
						
					}					
				}
			}	

			$this->itemPropertyCache[$item->itemBase->id] = $list;
		}
		
		return $this->itemPropertyCache[$item->itemBase->id];
	}
}
