<?hh // strict
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;


class DefaultPriceSearch extends CSVGenerator
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
     * DefaultPriceSearch constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ElasticExportHelper $elasticExportHelper, ArrayHelper $arrayHelper)
    {
        $this->elasticExportHelper = $elasticExportHelper;
        $this->arrayHelper = $arrayHelper;
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
					'article_id',
                    'deeplink',
                    'name',
                    'short_description',
                    'description',
                    'article_no',
                    'producer',
                    'model',
                    'availability',
                    'ean',
                    'isbn',
                    'fedas',
                    'unit',
                    'price',
                    'price_old',
                    'weight',
                    'category1',
                    'category2',
                    'category3',
                    'category4',
                    'category5',
                    'category6',
                    'category_concat',
                    'image_url_preview',
                    'image_url',
                    'shipment_and_handling',
                    'unit_price',
                    'unit_price_value',
                    'unit_price_lot',
			    ]);

			foreach($resultData as $item)
			{
				$data = [
					'article_id'            => $item->itemBase->id,
                    'deeplink'              => $this->elasticExportHelper->getUrl($item, $settings, true, false),
                    'name'                  => $this->elasticExportHelper->getName($item, $settings),
                    'short_description'     => $item->itemDescription->shortDescription,
                    'description'           => $this->elasticExportHelper->getDescription($item, $settings, 256),
                    'article_no'            => $item->variationBase->customNumber,
                    'producer'              => $item->itemBase->producer,
                    'model'                 => $item->variationBase->model,
                    'availability'          => $this->elasticExportHelper->getAvailability($item, $settings),
                    'ean'                   => $item->variationBarcode->code,
                    'isbn'                  => $item->variationBarcode->code, //TODO isbn
                    'fedas'                 => $item->itemBase->fedas,
                    'unit'                  => '',
                    'price'                 => number_format($this->elasticExportHelper->getPrice($item, $settings), 2, '.', ''),
                    'price_old'             => '',//TODO UVP
                    'weight'                => $item->variationBase->weightG,
                    'category1'             => '',//TODO category1
                    'category2'             => '',//TODO category2
                    'category3'             => '',//TODO category3
                    'category4'             => '',//TODO category4
                    'category5'             => '',//TODO category5
                    'category6'             => '',//TODO category6
                    'category_concat'       => $this->elasticExportHelper->getCategory($item, $settings),
                    'image_url_preview'     => $this->elasticExportHelper->getImages($item, $settings, ';', 'preview'),
                    'image_url'             => $this->elasticExportHelper->getMainImage($item, $settings),
                    'shipment_and_handling' => number_format($this->elasticExportHelper->getShippingCost($item, $settings), 2, ',', ''),
                    'unit_price'            => $this->elasticExportHelper->getBasePrice($item, $settings),
                    'unit_price_value'      => '',
                    'unit_price_lot'        => ''
				];

				$this->addCSVContent(array_values($data));
			}
        }
    }
}